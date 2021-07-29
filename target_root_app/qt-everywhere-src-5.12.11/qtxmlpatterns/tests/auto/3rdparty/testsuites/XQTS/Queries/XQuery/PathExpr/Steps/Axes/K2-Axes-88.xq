(:*******************************************************:)
(: Test: K2-Axes-88                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure node sorting is applied when the left operand is the result of the comma operator, while using variable references. :)
(:*******************************************************:)
declare variable $i := <e>
    <a>1</a>
    <b>2</b>
</e>;
<result>
    {
        ($i/b, $i/a)/.
    }
</result>