(: Name: fn-resolve-uri-22 :)
(: Description: Evaluation of resolve-uri function with relative argument set to an absolute URI and given as an argument to fn:substring-before function (two argument version of function).:)
(: Use the fn-string function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:resolve-uri(fn:substring-before("http://www.example.com123","123"),""))