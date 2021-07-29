(: Name: constprolog-13 :)
(: Description: Evaluation of a prolog with construction declaration set to "preserve" for a computed element (inside of instance of expression) and used as argument to fn:not.:)

declare construction preserve;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:not(element someElement{"some content"} instance of element(*,xs:anyType))