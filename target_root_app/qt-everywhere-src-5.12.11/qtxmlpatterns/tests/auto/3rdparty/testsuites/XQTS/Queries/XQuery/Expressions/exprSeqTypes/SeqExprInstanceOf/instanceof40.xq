(: Name: instanceof40 :)
(: purpose: Evaluation of "instance of" expression for pattern "boolean instance of xs:anyURI".:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)
xs:boolean("true") instance of xs:anyURI
