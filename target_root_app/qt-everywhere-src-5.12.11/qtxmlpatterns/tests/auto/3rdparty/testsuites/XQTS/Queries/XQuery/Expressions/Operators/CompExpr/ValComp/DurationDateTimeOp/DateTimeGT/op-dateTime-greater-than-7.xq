(:*******************************************************:)
(:Test: op-dateTime-greater-than-7                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dateTime-greater-than" function used  :)
(:together with "and" expression (gt operator).          :)
(:*******************************************************:)
 
(xs:dateTime("2002-04-02T12:00:00Z") gt xs:dateTime("2002-04-02T12:01:00Z")) and (xs:dateTime("2003-04-02T12:00:00Z") gt xs:dateTime("2002-04-02T12:00:00Z"))