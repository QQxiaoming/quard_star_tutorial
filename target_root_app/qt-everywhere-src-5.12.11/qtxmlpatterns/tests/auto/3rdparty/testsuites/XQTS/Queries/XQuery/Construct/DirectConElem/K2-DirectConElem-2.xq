(:*******************************************************:)
(: Test: K2-DirectConElem-2                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure comments aren't included when extracting the string-value from elements. :)
(:*******************************************************:)
string(<a attr="content"><!-- NOTINC -->1<b>2<c><!-- NOTINC -->34</c><!-- NOTINC --><d/>56</b>7</a>)
    eq "1234567"