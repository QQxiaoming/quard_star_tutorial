(: Name: fn-resolve-uri-19 :)
(: Description: Evaluation of resolve-uri function with relative argument set to an absolute URI and given as an argument to fn:substring function (two argument version of function).:)
(: Use the fn-string function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:resolve-uri(fn:substring("123http://www.examples.com",4),""))