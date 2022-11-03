def pad(number, digits):
    """format input number to string and pad with leading zeros"""
    s = str(number)
    n = len(s)
    if len(s) >= digits:
        return s
    zeros = "0"*(digits - len(s))
    ret = zeros + s
    if len(ret) != digits:
        fail('pad zeros function failed')
    return ret
