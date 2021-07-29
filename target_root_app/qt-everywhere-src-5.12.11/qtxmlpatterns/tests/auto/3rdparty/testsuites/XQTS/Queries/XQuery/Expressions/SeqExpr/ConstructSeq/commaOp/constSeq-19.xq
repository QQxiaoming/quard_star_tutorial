(: Name: constSeq-19:)
(: Description: Constructing sequences.  Constructing a sequence, where one of its members contains invocation to "xs:boolean()".:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(xs:boolean(fn:true()), fn:false(), fn:true())