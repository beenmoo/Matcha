namespace Matcha
{
class Entity
{
public:
    Entity() = default;
    virtual ~Entity() = default;

    virtual void Update();
    void SetActive(bool active)
    {
        m_Active = active;
    }
    [[nodiscard]] bool IsActive() const
    {
        return m_Active;
    }

private:
    bool m_Active = true;
};
}  // namespace Matcha