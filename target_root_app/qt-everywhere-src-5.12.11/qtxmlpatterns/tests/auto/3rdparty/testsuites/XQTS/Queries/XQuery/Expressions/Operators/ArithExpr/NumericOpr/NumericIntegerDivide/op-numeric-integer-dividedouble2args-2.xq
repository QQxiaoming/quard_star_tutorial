(:*******************************************************:)
(:Test: op-numeric-integer-dividedouble2args-2           :)
(:Written By: Sorin Nasoi                                :)
(:Date: 2009-04-01+02:00                                 :)
(:Purpose: "op:numeric-integer-divide" with both         :)
(: arguments of type xs:double. Raise [err:FOAR0002]     :) 
(:*******************************************************:)

xs:double("1") idiv xs:double("NaN")