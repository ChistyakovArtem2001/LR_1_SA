import pandas as pd
import json
from sqlalchemy import create_engine, MetaData, Table, Column, Integer, String, VARCHAR

# engine = create_engine("postgresql+psycopg2://stud:stud@localhost/archdb", echo = False)
# metadata = MetaData()
# users_table = Table('users', metadata,
#                     Column('id', Integer, primary_key=True, autoincrement=True),
#                     Column('first_name', VARCHAR(256)),
#                     Column('last_name', VARCHAR(256)),
#                     Column('login', VARCHAR(256)),
#                     Column('password', VARCHAR(256)),
#                     Column('email', VARCHAR(256)),
#                     Column('title', VARCHAR(1024))
#                   
#                     )

# metadata.create_all(engine)
# df = pd.read_json("hl_mai_lab_01/ExportJson.json")
# df.to_sql('users', con=engine, if_exists='replace', index=False)

metadata = MetaData()

users_table = Table('users', metadata,
                    Column('id', Integer, primary_key=True),
                    Column('first_name', String(256), nullable=False),
                    Column('last_name', String(256), nullable=False),
                    Column('login', String(256), nullable=False),
                    Column('password', String(256), nullable=False),
                    Column('email', String(256),nullable=False),
                    Column('title', String(1024))
                    )


engine = create_engine("postgresql+psycopg2://stud:stud@localhost/archdb", echo = False)
conn = engine.connect()

with open('hl_mai_lab_01/ExportJson.json', 'r') as file:
    data = json.load(file)
for record in data:
    insert_query = users_table.insert().values(
        first_name=record['first_name'],
        last_name=record['last_name'],
        login=record['login'],
        password=record['password'],
        email=record.get('email'), 
        title=record.get('title')    
        
    )
    
    conn.execute(insert_query)
    conn.commit()

conn.close()