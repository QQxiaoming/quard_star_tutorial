(: Name: op-qName-equal-11 :)
(: Description: Evaluation of op-QName-equal operator with two same namespace uri, different local part, different prefix. Uses the "eq" operator. :)
(: Should ignore the prefix :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:QName("http://www.example.com/example", "px1:person1") eq fn:QName("http://www.example.com/example","px2:person2")