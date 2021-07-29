(: Name: op-qName-equal-21 :)
(: Description: Evaluation of op-QName-equal operator as an argument to the fn:not function.  Uses "eq" operator :)
(: Should ignore the prefix :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:not(fn:QName("http://www.example.com/example", "px1:person1") eq fn:QName("http://www.example.com/example","px2:person2"))