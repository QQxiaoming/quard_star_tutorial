(: Name: fn-resolve-uri-14 :)
(: Description: Evaluation of resolve-uri function with relative argument set to an absolute URI and given as a an argument to fn:substring-before function (Two argument version of function).:)
(: Use the fn:substring-before function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:resolve-uri("examples",fn:substring-before("http://www.example.com/123","123")))