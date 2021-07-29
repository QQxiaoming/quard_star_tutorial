(: Name: fn-resolve-uri-7 :)
(: Description: Evaluation of resolve-uri function with relative argument set to an absolute URI and given as a an argument to xs:string function (Two argument version of function).:)
(: Use the fn-string function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:resolve-uri("examples",xs:string("http://www.examples.com/")))