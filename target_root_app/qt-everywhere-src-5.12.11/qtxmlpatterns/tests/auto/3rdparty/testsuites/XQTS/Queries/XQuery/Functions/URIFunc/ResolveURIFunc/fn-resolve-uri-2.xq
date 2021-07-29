(: Name: fn-resolve-uri-2 :)
(: Description: Evaluation of resolve-uri function with relative argument set to zero length string.  Use the base-uri property that is set.:)
(: Use the fn-string function :)

declare base-uri "http://www.example/";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:resolve-uri(""))