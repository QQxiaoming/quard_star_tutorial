(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-14                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply fn:avg() to the return value of a for clause with sorting. :)
(:*******************************************************:)
declare variable $e := <e>
    <a>3</a>
    <a>2</a>
    <a>1</a>
</e>;
<result>
    {
        avg(for $i in $e/a
            order by $i
            return $i)
    }
</result>