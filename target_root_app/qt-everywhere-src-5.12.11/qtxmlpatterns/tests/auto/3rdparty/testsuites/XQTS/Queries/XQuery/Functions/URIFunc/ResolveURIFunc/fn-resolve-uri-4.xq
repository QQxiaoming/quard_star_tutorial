(: Name: fn-resolve-uri-4 :)
(: Description: Evaluation of resolve-uri function with an invalid URI value for second argument.:)
(: Use the fn-string function :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:resolve-uri("examples","http:%%"))
