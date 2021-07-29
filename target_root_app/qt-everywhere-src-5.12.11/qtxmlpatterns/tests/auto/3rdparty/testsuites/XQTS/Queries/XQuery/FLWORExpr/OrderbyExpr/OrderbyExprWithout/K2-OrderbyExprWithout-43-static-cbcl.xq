(:*******************************************************:)
(: Test: K2-OrderbyExprWithout-43                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Extract the effective boolean value from the result of order by. :)
(:*******************************************************:)
declare construction strip;
if(for $i in <e>
                <a id="3"/>
                <b id="2"/>
                <c id="1"/>
                </e>/*
    order by xs:integer($i/@id)
    return $i)
then 4
else 9
