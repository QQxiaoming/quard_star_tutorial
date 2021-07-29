(: Name: fn-resolve-uri-3 :)
(: Description: Evaluation of resolve-uri function with an invalid URI value for first argument.:)
(: Use the fn-string function :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:resolve-uri(":","http://www.example.com/"))