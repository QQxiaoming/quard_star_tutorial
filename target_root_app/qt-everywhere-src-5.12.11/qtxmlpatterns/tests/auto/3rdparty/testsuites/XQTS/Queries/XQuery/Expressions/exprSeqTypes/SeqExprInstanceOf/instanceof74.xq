(: Name: instanceof74 :)
(: purpose: Evaluation of "instance of" expression for pattern "anyURI instance of xs:string".:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)
xs:anyURI("http://www.example.com") instance of xs:string
