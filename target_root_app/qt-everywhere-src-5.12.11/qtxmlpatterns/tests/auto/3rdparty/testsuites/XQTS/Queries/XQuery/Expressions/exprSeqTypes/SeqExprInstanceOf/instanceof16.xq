(: Name: instanceof16 :)
(: purpose: Evaluation of "instance of" expression for pattern "time instance of xs:float".:)
(: insert-start :)
declare variable $input-context external;
(: insert-end :)
xs:time("13:20:10.5Z") instance of xs:float
