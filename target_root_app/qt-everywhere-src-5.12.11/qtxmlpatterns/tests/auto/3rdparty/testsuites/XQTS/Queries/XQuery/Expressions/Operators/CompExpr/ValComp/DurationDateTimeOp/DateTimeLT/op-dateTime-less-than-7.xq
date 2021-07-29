(:*******************************************************:)
(:Test: op-dateTime-less-than-7                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dateTime-less-than" function used  :)
(:together with "and" expression (lt operator).          :)
(:*******************************************************:)
 
(xs:dateTime("2002-04-02T12:00:00Z") lt xs:dateTime("2002-04-02T12:01:00Z")) and (xs:dateTime("2003-04-02T12:00:00Z") lt xs:dateTime("2002-04-02T12:00:00Z"))