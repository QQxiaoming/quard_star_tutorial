(:*******************************************************:)
(: Test: DirectConElemNamespace-5                        :)
(: Written by: Sorin Nasoi                               :)
(: Date: 2009-04-01+02:00                                :)
(: Purpose: Use the ''-escape mechanism.                 :)
(: Modified by MHK to avoid generating output that won't canonicalize - bug 6868 :)
(:*******************************************************:)

namespace-uri(<p:e xmlns:p='http://ns.example.com/ns?val=''''''asd'/>)