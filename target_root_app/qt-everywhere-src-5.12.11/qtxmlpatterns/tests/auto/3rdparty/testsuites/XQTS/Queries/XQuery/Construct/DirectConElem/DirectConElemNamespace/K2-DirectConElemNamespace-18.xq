(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-18                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A namespace must be a literal, for which bracket-escapes are invalid(#2). :)
(:*******************************************************:)
<e xmlns:p="content{()}"/>