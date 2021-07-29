(: Name: op-qName-equal-13 :)
(: Description: Evaluation of op-QName-equal operator with two different namespace uri, different local part, different prefix. Uses the "eq" operator. :)
(: Should ignore the prefix :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:QName("http://www.example.com/example1", "px1:person1") eq fn:QName("http://www.example.com/example2","px2:person2")