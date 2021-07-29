(: Name: CastFOCA0001:)
(: Description: casting an xs:decimal with a value to big for that type.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:decimal(99e100)