(: Name: treat-as-25 :)
(: Description: Evaluation of treat as expression used as part of an subtraction operation.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := (100+200) div 10
return
  ($var cast as xs:integer treat as xs:integer) - 10