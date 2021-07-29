(:*******************************************************:)
(: Test: K-NodeNumberFunc-12                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: fn:number() applied to a type which a cast regardless of source value never would succeed for. :)
(:*******************************************************:)
string(number(xs:anyURI("example.com/"))) eq "NaN"