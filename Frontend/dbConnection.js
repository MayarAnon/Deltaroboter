const { MongoClient } = require('mongodb');

// Replace the connection string with your own MongoDB URI
const uri = 'mongodb://localhost:27017';

async function insertDocuments() {
  const client = new MongoClient(uri, { useNewUrlParser: true, useUnifiedTopology: true });
  try {
    await client.connect();
    console.log('Connected to MongoDB');

    const db = client.db('Lernen');
    const collection = db.collection('Einkaufsliste');

    // Insert one document
    const result = await collection.insertOne({ name: 'John', age: 30 });
    console.log(`Inserted ${result.insertedCount} document`);

    // Insert multiple documents
    const docs = [
      { name: 'Jane', age: 25 },
      { name: 'Bob', age: 40 },
      { name: 'Alice', age: 35 },
    ];
    const result2 = await collection.insertMany(docs);
    console.log(`Inserted ${result2.insertedCount} documents`);

  } catch (err) {
    console.error(err);
  } finally {
    await client.close();
    console.log('Disconnected from MongoDB');
  }
}



async function getAllDocuments() {
  const client = new MongoClient(uri, { useNewUrlParser: true, useUnifiedTopology: true });
  try {
    await client.connect();
    console.log('Connected to MongoDB');

    const db = client.db('Lernen');
    const collection = db.collection('Einkaufsliste');

    // Retrieve all documents
    let object = [];
    const cursor = collection.find({});
    await cursor.forEach((item) =>{object.push(item)});
    return object;
  } catch (err) {
    console.error(err);
  } finally {
    await client.close();
    console.log('Disconnected from MongoDB');
  }
}




module.exports = getAllDocuments()