import logging
from flask import Flask, request

log = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)

app = Flask(__name__)

@app.route("/api/foo", methods=['POST'])
def foo():
    if request.content_type == 'application/json':
        print(request.json)
    return '', 200


if __name__ == "__main__":
    app.run()



