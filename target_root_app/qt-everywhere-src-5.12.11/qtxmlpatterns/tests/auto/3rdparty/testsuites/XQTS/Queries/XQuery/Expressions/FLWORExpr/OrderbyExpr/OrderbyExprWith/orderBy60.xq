(:*******************************************************:)
(: Test: orderBy60                                       :)
(: Written by: Sorin Nasoi                               :)
(: Date: 2009-05-01+02:00                                :)
(: Purpose: Evaluation of "order by" clause using a      :)
(: relative, valid collation.                            :)
(:*******************************************************:)

(: insert-start :)
declare base-uri "http://www.w3.org/2005/xpath-functions/";
declare default element namespace "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)


<results> {
for $x in $input-context1/DataValues/Strings/orderData
order by concat($x,"()") ascending  collation "collation/codepoint"
return concat($x,"()")
}
</results>
