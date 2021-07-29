(: Name: instanceof71 :)
(: purpose: Evaluation of "instance of" expression for pattern "anyURI instance of xs:float".:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)
xs:anyURI("http://www.example.com") instance of xs:float
