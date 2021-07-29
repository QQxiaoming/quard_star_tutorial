(: Name: fn-resolve-uri-5 :)
(: Description: Evaluation of resolve-uri function with relative argument set to an absolute URI.:)
(: Use the fn-string function :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:resolve-uri("http://www.examples.com",""))