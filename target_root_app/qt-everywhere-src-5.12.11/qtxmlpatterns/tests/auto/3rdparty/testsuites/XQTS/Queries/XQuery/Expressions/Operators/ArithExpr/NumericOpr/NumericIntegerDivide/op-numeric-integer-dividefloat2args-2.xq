(:*******************************************************:)
(:Test: op-numeric-integer-dividefloat2args-2            :)
(:Written By: Sorin Nasoi                                :)
(:Date: 2009-04-01+02:00                                 :)
(:Purpose: Integer divide with operands of type xs:float.:)
(:Raise [err:FOAR0002].																	 :)
(:*******************************************************:)

xs:float("1") idiv xs:float("NaN")