(: Name: fn-base-uri-13 :)
(: Description: Evaluation of base-uri function with argument set to a directly constructed element node argument. Should not declared base uri property:)
(: Use fn:string :)

declare base-uri "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:base-uri(<anElement>some content</anElement>))