(: Name: fn-nilled-19 :)
(: Description: Evaluation of nilled function with argument set to a document node :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:nilled(document {<aList><anElement>data</anElement></aList>}))