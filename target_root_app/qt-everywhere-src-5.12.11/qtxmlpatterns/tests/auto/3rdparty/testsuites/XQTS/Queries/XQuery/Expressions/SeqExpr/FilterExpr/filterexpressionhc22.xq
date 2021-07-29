(: Name: filterexpressionhc22 :)
(: Description: Simple filter expression involving the "fn:false" function:)
(: Use fn;count to avoid empty sequence. :)

fn:count(((1,2,3,4,5,6,7,8,9,10,11)[fn:false()]))