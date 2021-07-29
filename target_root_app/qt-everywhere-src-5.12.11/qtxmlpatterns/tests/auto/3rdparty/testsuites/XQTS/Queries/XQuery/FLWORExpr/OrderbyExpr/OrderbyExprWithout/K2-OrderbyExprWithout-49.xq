(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-49                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Sort special floating point values(stable sort). :)
(:*******************************************************:)
let $numbers := (<e>NaN</e>, <e/>, <e/>, <e>NaN</e>, <e>NaN</e>, <e>INF</e>, <e>NaN</e>, <e/>, <e>3</e>, comment{"3"})
return
(for $i in $numbers stable order by xs:double($i/text()) empty least return xs:double($i/text()),
"SEP",
for $i in $numbers stable order by xs:double($i/text()) empty greatest return xs:double($i/text()))
