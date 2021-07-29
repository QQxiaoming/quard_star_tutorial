(:*******************************************************:)
(:Test: op-dateTime-greater-than-10                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dateTime-greater-than" function used  :)
(:together with "or" expression (ge operator).           :)
(:*******************************************************:)
 
(xs:dateTime("2002-04-03T12:00:10Z") ge xs:dateTime("1990-04-02T12:10:00Z")) or (xs:dateTime("1975-04-03T12:10:00Z") ge xs:dateTime("2000-02-02T12:00:09Z"))