(: Name: fn-base-uri-17 :)
(: Description: Evaluation of base-uri function with argument set to a computed constructed PI node argument. :)
(: Use fn:count to avoid empty file.  Should not use the declare base-uri :)

declare base-uri "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:base-uri(processing-instruction {"PItarget"} {"PIcontent"}))