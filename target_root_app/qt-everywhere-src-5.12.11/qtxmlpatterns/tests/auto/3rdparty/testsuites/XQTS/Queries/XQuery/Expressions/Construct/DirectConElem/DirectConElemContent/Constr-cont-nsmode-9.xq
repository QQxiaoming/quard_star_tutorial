(: Name: Constr-cont-nsmode-9 :)
(: Written by: Michael Kay :)
(: Description: Copy namespace sensitive content. See bug 4151. 
   This test succeeds: a namespace-sensitive element can be copied if copy-namespaces=preserve,
   and its namespace-sensitive attribute can be copied if its parent element is copied :)

declare namespace atomic = "http://www.w3.org/XQueryTest";
declare copy-namespaces preserve, inherit;
declare construction preserve;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<new>{ $input-context//atomic:QName }</new>
