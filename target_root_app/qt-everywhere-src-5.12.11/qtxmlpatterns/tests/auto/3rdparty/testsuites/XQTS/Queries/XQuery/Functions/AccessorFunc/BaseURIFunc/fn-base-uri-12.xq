(: Name: fn-base-uri-12 :)
(: Description: Evaluation of base-uri function with argument set to a computed constructed Document node argument. Uses declared base uri property:)
(: Use fn:string :)

declare base-uri "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:base-uri(document {<aDocument>some content</aDocument>}))