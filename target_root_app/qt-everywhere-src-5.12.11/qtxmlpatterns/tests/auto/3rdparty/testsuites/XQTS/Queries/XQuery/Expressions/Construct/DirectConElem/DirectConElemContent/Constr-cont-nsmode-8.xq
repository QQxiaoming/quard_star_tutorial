(: Name: Constr-cont-nsmode-8 :)
(: Written by: Michael Kay :)
(: Description: Copy namespace sensitive content. See bug 4151 :)

declare namespace atomic = "http://www.w3.org/XQueryTest";
declare copy-namespaces preserve, inherit;
declare construction preserve;

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<new>{ ($input-context//atomic:QName/@atomic:attr)[1] }</new>
