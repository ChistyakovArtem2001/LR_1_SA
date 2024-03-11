workspace {
    name "Сайт конференции"
    description "Система для организации и управления конференциями"

    !identifiers hierarchical

    !docs documentation
    !adrs decisions
    

    model {

        properties { 
            structurizr.groupSeparator "/"
        }
        
        // Определение пользователей и системы конференций
        user = person "Участник конференции"
        conference_platform = softwareSystem "Сайт конференции" {
            description "Сервер для управления конференциями и докладами"

            // Определение сервисов внутри системы конференций
            user_service = container "User service" {
                description "Сервис управления пользователями"
            }

            conference_service = container "Conference service" {
                description "Сервис управления конференциями"
            }

            presentation_service = container "Presentation service" {
                description "Сервис управления докладами"
            }

            // Определение баз данных и их взаимодействие с сервисами
            group "Слой данных" {
                user_database = container "User Database" {
                    description "База данных с данными пользователей"
                    technology "PostgreSQL 16"
                    tags "database"
                }

                user_cache = container "User Cache" {
                    description "Кэш пользовательских данных"
                    technology "Redis 7.2"
                    tags "database"
                }

                conference_database = container "Conference Database" {
                    description "База данных с информацией о конференциях и докладах"
                    technology "MongoDB 7.0.6"
                    tags "database"
                }
            }

            // Установление связей между сервисами и базами данных
            user_service -> user_cache "Получение/обновление данных о пользователях" "TCP 6379"
            user_service -> user_database "Получение/обновление данных о пользователях" "TCP 5432"

            conference_service -> conference_database "Взаимодействие с базой данных конференций" "TCP 27018"
            conference_service -> user_service "Аутентификация пользователя" "REST HTTP 443"
            presentation_service -> user_service "Аутентификация пользователя" "REST HTTP 443"
          
            presentation_service -> conference_database "Взаимодействие с базой данных докладов" "TCP 27018"

            // Установление связей между пользователем и системой конференций
            user -> conference_platform "Участие в конференциях и докладах" "REST HTTP:8080" 
            user -> conference_service "Создание конференции" "REST HTTP:8080"
            user -> presentation_service "Создание доклада " "REST HTTP:8080"
            user -> user_service "Регистрация нового участника" "REST HTTP:8080"
        
        }

        // Развертывание сервисов и баз данных
        deploymentEnvironment "Production" {
            deploymentNode "User Server" {
                containerInstance conference_platform.user_service
            }

            deploymentNode "Conference Server" {
                containerInstance conference_platform.conference_service
            }
             deploymentNode "Presentation Server" {
                containerInstance conference_platform.presentation_service
            }

            deploymentNode "Database Servers" {
     
                deploymentNode "Database Server 1" {
                    containerInstance conference_platform.user_database
                }

                deploymentNode "Database Server 2" {
                    containerInstance conference_platform.conference_database
                    instances 3
                }

                deploymentNode "Cache Server" {
                    containerInstance conference_platform.user_cache
                }
            }
            
        }
    }

    views {
        themes default 

        properties { 
            structurizr.tooltips true
        }


        !script groovy {
            workspace.views.createDefaultViews()
            workspace.views.views.findAll { it instanceof com.structurizr.view.ModelView }.each { it.enableAutomaticLayout() }
        }

        // Диаграммы сценариев использования
        dynamic conference_platform "UC01" "Добавление нового участника" {
            autoLayout
            user -> conference_platform.user_service "Создание нового участника (POST /user)"
            conference_platform.user_service -> conference_platform.user_database "Сохранение данных о участнике" 
        }

        dynamic conference_platform "UC02" "Удаление участника" {
            autoLayout
            user -> conference_platform.user_service "Удаление участника (DELETE /user)" 
            conference_platform.user_service -> conference_platform.user_database "Удаление данных о пользователе" 
        }

        dynamic conference_platform "UC03" "Создание нового доклада" {
            autoLayout
            user -> conference_platform.presentation_service "Создание нового доклада (POST /presentation)"
            conference_platform.presentation_service -> conference_platform.user_service "Проверка аутентификации участника (GET /user)"
            conference_platform.presentation_service -> conference_platform.conference_database "Сохранение доклада" 
        }

        dynamic conference_platform "UC04" "Получение списка всех докладов" {
            autoLayout
            user -> conference_platform.presentation_service "Получение списка всех докладов (GET /presentation)"
            conference_platform.presentation_service -> conference_platform.user_service "Проверка аутентификации участника (GET /user)"
            conference_platform.presentation_service -> conference_platform.conference_database "Получение списка всех докладов" 
        }

        dynamic conference_platform "UC05" "Добавление доклада в конференцию" {
            autoLayout
            user -> conference_platform.conference_service "Добавление доклада в конференцию (POST /conference)"
            conference_platform.conference_service -> conference_platform.user_service "Проверка аутентификации участника (GET /user)"
            conference_platform.conference_service -> conference_platform.conference_database "Добавление доклада в конференцию" 
        }

        dynamic conference_platform "UC06" "Получение списка докладов на конференции" {
            autoLayout
            user -> conference_platform.conference_service "Получение списка докладов на конференции (GET /conference)"
            conference_platform.conference_service -> conference_platform.user_service "Проверка аутентификации участника (GET /user)"
            conference_platform.conference_service -> conference_platform.conference_database "Получение списка докладов на конференции" 
        }


        // Стилизация элементов БД как цилиндры
        styles {
            element "database" {
                shape cylinder
            }
        }
        
    }
}
