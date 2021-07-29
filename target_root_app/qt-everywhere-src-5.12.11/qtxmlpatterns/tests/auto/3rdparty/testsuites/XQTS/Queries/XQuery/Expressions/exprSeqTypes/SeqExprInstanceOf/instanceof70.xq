(: Name: instanceof70 :)
(: purpose: Evaluation of "instance of" expression for pattern "anyURI instance of xs:boolean".:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)
xs:anyURI("http://www.example.com") instance of xs:boolean
