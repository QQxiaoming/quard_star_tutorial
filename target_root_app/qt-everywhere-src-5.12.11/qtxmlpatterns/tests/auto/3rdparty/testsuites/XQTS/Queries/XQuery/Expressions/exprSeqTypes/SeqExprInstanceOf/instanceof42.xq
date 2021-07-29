(: Name: instanceof42 :)
(: purpose: Evaluation of "instance of" expression for pattern "boolean instance of xs:decimal".:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)
xs:boolean("true") instance of xs:decimal
