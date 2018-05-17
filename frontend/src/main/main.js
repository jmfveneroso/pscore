import React, {Component} from 'react';
import {Link, Switch, Route, Redirect} from 'react-router-dom';
import {Container} from 'reactstrap';

class Main extends Component {
  constructor(props) {
    super(props);
  }

  render() {
    return (
      <div className="app">
        <div className="app-body">
          <main className="main">
            <Container fluid>
	      <div>Example 2</div>
            </Container>
          </main>
        </div>
      </div>
    );
  }
}

export default Main;
