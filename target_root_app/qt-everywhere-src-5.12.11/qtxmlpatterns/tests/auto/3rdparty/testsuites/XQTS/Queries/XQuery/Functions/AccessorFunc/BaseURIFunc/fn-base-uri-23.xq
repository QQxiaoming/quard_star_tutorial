(: Name: fn-base-uri-23 :)
(: Description: Evaluation of base-uri function with argument set to a directly constructed Element node with base-xml :)
(: argument that needs escaping. Uses fn:string. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:base-uri(<anElement xml:base="http://example.com/examples">Element content</anElement>))
