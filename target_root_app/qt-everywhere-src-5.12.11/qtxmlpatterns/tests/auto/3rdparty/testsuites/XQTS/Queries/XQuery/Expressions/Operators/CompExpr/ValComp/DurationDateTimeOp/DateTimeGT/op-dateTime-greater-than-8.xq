(:*******************************************************:)
(:Test: op-dateTime-greater-than-8                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dateTime-greater-than" function used  :)
(:together with "and" expression (ge operator).          :)
(:*******************************************************:)
 
(xs:dateTime("2002-04-02T12:00:00Z") ge xs:dateTime("2005-04-02T12:00:20Z")) and (xs:dateTime("2002-04-02T12:10:00Z") ge xs:dateTime("2002-04-03T12:00:00Z"))