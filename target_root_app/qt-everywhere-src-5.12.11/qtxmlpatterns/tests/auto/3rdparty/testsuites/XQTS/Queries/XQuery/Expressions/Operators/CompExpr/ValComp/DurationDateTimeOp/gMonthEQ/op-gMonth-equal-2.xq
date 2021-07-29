(:*******************************************************:)
(:Test: op-gMonth-equal-2                                :)
(:Date: June 16, 2005                                    :)
(:Purpose: Evaluates The "gMonth-equal" function         :)
(:As per example 2 (for this function) of the F&O  specs :)
(:*******************************************************:)

(xs:gMonth("--12-05:00") eq xs:gMonth("--12Z"))