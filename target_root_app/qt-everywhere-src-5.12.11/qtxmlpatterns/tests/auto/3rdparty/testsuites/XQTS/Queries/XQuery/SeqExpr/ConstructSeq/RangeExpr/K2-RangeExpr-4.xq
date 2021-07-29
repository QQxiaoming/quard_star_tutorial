(:*******************************************************:)
(: Test: K2-RangeExpr-4                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Expressions that are tricky for local rewrites. :)
(:*******************************************************:)
1 to <value>5</value>,
    5 to <value>5</value>,
    <value>1</value> to 5,
    <value>1</value> to <value>5</value>,
    let $i := <e>5</e>
    return $i to $i,
    count(5 to 10),
    count(1000 to 2000),
    count(<e>5</e> to 10),
    count(3 to <e>10</e>),
    count(<e>3</e> to <e>10</e>),
    count(<e>5</e> to 10),
    count(3 to <e>10</e>),
    count(<e>3</e> to <e>10</e>),
    count(4294967295 to 4294967298)