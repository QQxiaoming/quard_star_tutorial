(: Name: constSeq-20:)
(: Description: Constructing sequences.  Constructing a sequence, where both of its members contains invocation to "xs:date()".:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(xs:date("2004-12-25Z"),xs:date("2004-12-26Z"))