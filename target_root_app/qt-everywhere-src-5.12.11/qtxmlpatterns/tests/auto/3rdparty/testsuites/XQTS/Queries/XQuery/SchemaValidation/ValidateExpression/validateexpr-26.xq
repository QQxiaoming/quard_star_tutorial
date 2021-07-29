(: name : validateexpr-26 :)
(: Description: Test For error condition XQDY0061 using a document node.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

validate { document { <a/>, <b/> } }