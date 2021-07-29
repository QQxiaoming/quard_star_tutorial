(:*******************************************************:)
(: Test: K2-NodeTest-41                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use fn:last() with nested predicates.        :)
(:*******************************************************:)
<a><b name="C"/><b name= "D"/></a>//b[@name="D"][last() = 1]