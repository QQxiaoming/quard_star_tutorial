(: Name: constprolog-12 :)
(: Description: Evaluation of a prolog with construction declaration set to "strip" for a directly constructed element (inside of instance expression) :)
(: and used as argument to fn:not() .:)

declare construction strip;

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:not(<someElement>some content</someElement> instance of element(*,xs:untyped))