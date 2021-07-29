(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-15                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that cardinality checks are effective on the return value of a . :)
(:*******************************************************:)
declare variable $e := <e>
    <a>3</a>
    <a>2</a>
    <a>1</a>
</e>;
exactly-one(for $i in $e/a
            order by $i
            return $i)