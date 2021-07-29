(: Name: op-qName-equal-24 :)
(: Description: Evaluation of op-QName-equal operator as part of a boolean expression.  Uses "ne"  and "and" operators :)
(: Should ignore the prefix :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(fn:QName("http://www.example.com/example", "px:person") ne fn:QName("http://www.example.com/example","px:person")) and fn:true()