(: Name: instanceof36 :)
(: purpose: Evaluation of "instance of" expression for pattern "boolean instance of xs:date".:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)
xs:boolean("true") instance of xs:date
