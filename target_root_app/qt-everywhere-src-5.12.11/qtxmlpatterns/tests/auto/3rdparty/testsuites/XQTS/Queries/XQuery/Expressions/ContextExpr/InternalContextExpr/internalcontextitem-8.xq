(: Name: internalcontextitem-8 :)
(: Description: context item expression where context item is an xs:boolean used with fn:not(). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(fn:false(),fn:true(),fn:false())[fn:not(xs:boolean(.))]