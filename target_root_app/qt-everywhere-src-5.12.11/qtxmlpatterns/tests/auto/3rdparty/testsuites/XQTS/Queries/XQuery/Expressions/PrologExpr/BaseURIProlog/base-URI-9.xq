(: Name: base-URI-9 :)
(: Description: Evaluates base-uri with the fn:static-base-uri function.  Typical usage:)

declare base-uri "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())