(: Name: constSeq-22:)
(: Description: Constructing sequences.  Constructing a sequence, where both of its members contains invocation to "xs:time()".:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(xs:time("08:00:00+09:00"),xs:time("08:00:00+10:00"))