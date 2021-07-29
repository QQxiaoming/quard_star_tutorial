xquery version "1.1";
(: Higher Order Functions :)
(: filter function :)
(: Author - Michael Kay, Saxonica :)

declare variable $data := 
  <employees>
    <emp name="john" salary="120"/>
    <emp name="mary" salary="310"/>
    <emp name="anne" salary="250"/>
    <emp name="kumar" salary="290"/>
  </employees>;

<out>{filter(function($x as element(emp)){xs:int($x/@salary) lt 300}, $data/emp)}</out>