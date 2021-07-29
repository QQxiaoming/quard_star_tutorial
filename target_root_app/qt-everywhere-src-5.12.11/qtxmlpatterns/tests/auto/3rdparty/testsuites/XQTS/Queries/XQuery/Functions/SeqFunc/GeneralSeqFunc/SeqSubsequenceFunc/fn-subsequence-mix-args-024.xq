(:*******************************************************:) 
(: Test: fn-subsequence-mix-args-024.xq                  :) 
(: Written By: Michael Kay                               :) 
(: Date: 18 June 2010                                    :) 
(: Purpose: subsequence(X, -INF, +INF                    :) 
(:*******************************************************:) 

(: Returns () because -INF + INF = NaN, and position() lt NaN is false.
   See XSLT test case bug 837 (member only bugzilla)  :)
 
count(fn:subsequence (1 to 10, xs:double('-INF'), xs:double('INF'))) 
